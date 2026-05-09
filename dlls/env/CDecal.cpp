#include "extdll.h"
#include "util.h"
#include "decals.h"
#include "CBasePlayer.h"

//
// Keep decal_e enum in sync with this
//
DLL_DECALLIST gDecals[] = {
	{ "{shot1",	0 },		// DECAL_GUNSHOT1 
	{ "{shot2",	0 },		// DECAL_GUNSHOT2
	{ "{shot3",0 },			// DECAL_GUNSHOT3
	{ "{shot4",	0 },		// DECAL_GUNSHOT4
	{ "{shot5",	0 },		// DECAL_GUNSHOT5
	{ "{lambda01", 0 },		// DECAL_LAMBDA1
	{ "{lambda02", 0 },		// DECAL_LAMBDA2
	{ "{lambda03", 0 },		// DECAL_LAMBDA3
	{ "{lambda04", 0 },		// DECAL_LAMBDA4
	{ "{lambda05", 0 },		// DECAL_LAMBDA5
	{ "{lambda06", 0 },		// DECAL_LAMBDA6
	{ "{scorch1", 0 },		// DECAL_SCORCH1
	{ "{scorch2", 0 },		// DECAL_SCORCH2
	{ "{blood1", 0 },		// DECAL_BLOOD1
	{ "{blood2", 0 },		// DECAL_BLOOD2
	{ "{blood3", 0 },		// DECAL_BLOOD3
	{ "{blood4", 0 },		// DECAL_BLOOD4
	{ "{blood5", 0 },		// DECAL_BLOOD5
	{ "{blood6", 0 },		// DECAL_BLOOD6
	{ "{yblood1", 0 },		// DECAL_YBLOOD1
	{ "{yblood2", 0 },		// DECAL_YBLOOD2
	{ "{yblood3", 0 },		// DECAL_YBLOOD3
	{ "{yblood4", 0 },		// DECAL_YBLOOD4
	{ "{yblood5", 0 },		// DECAL_YBLOOD5
	{ "{yblood6", 0 },		// DECAL_YBLOOD6
	{ "{break1", 0 },		// DECAL_GLASSBREAK1
	{ "{break2", 0 },		// DECAL_GLASSBREAK2
	{ "{break3", 0 },		// DECAL_GLASSBREAK3
	{ "{bigshot1", 0 },		// DECAL_BIGSHOT1
	{ "{bigshot2", 0 },		// DECAL_BIGSHOT2
	{ "{bigshot3", 0 },		// DECAL_BIGSHOT3
	{ "{bigshot4", 0 },		// DECAL_BIGSHOT4
	{ "{bigshot5", 0 },		// DECAL_BIGSHOT5
	{ "{spit1", 0 },		// DECAL_SPIT1
	{ "{spit2", 0 },		// DECAL_SPIT2
	{ "{bproof1", 0 },		// DECAL_BPROOF1
	{ "{gargstomp", 0 },	// DECAL_GARGSTOMP1,	// Gargantua stomp crack
	{ "{smscorch1", 0 },	// DECAL_SMALLSCORCH1,	// Small scorch mark
	{ "{smscorch2", 0 },	// DECAL_SMALLSCORCH2,	// Small scorch mark
	{ "{smscorch3", 0 },	// DECAL_SMALLSCORCH3,	// Small scorch mark
	{ "{mommablob", 0 },	// DECAL_MOMMABIRTH		// BM Birth spray
	{ "{mommablob", 0 },	// DECAL_MOMMASPLAT		// BM Mortar spray?? need decal
	
	// TODO: these aren't in decals.wad, why are they here?
	{ "{spr_splt1", 0 },	// DECAL_SPR_SPLT1
	{ "{spr_splt2", 0 },	// DECAL_SPR_SPLT2
	{ "{spr_splt3", 0 },	// DECAL_SPR_SPLT3
	{ "{ofscorch1", 0 },	// DECAL_OFSCORCH1
	{ "{ofscorch2", 0 },	// DECAL_OFSCORCH2
	{ "{ofscorch3", 0 },	// DECAL_OFSCORCH3
	{ "{ofsmscorch1", 0 },	// DECAL_OFSMSCORCH1
	{ "{ofsmscorch2", 0 },	// DECAL_OFSMSCORCH2
	{ "{ofsmscorch3", 0 },	// DECAL_OFSMSCORCH3
	
	// from valve/decals.wad
	{ "reflect1", 0 },
	{ "{247", 0 },
	{ "{64#0", 0 },
	{ "{64#1", 0 },
	{ "{64#2", 0 },
	{ "{64#3", 0 },
	{ "{64#4", 0 },
	{ "{64#5", 0 },
	{ "{64#6", 0 },
	{ "{64#7", 0 },
	{ "{64#8", 0 },
	{ "{64#9", 0 },
	{ "{ammo", 0 },
	{ "{arrow2a", 0 },
	{ "{arrow2b", 0 },
	{ "{arrow2c", 0 },
	{ "{arrow2d", 0 },
	{ "{arrow_l", 0 },
	{ "{arrow_r", 0 },
	{ "{bigblood1", 0 },
	{ "{bigblood2", 0 },
	{ "{biohaz", 0 },
	{ "{blood7", 0 },
	{ "{blood8", 0 },
	{ "{bloodhand1", 0 },
	{ "{bloodhand2", 0 },
	{ "{bloodhand3", 0 },
	{ "{bloodhand4", 0 },
	{ "{bloodhand5", 0 },
	{ "{bloodhand6", 0 },
	{ "{break", 0 },
	{ "{c1a1b", 0 },
	{ "{c1a3_grn1", 0 },
	{ "{c1a3_org1", 0 },
	{ "{c1a3_org2", 0 },
	{ "{c1a3_org3", 0 },
	{ "{c1a3_red1", 0 },
	{ "{c1a3_red2", 0 },
	{ "{c1a3_red3", 0 },
	{ "{c2a2_a", 0 },
	{ "{c2a2_b", 0 },
	{ "{c2a5g", 0 },
	{ "{capsa", 0 },
	{ "{capsb", 0 },
	{ "{capsc", 0 },
	{ "{capsd", 0 },
	{ "{capse", 0 },
	{ "{capsf", 0 },
	{ "{capsg", 0 },
	{ "{capsh", 0 },
	{ "{capsi", 0 },
	{ "{capsj", 0 },
	{ "{capsk", 0 },
	{ "{capsl", 0 },
	{ "{capsm", 0 },
	{ "{capsn", 0 },
	{ "{capso", 0 },
	{ "{capsp", 0 },
	{ "{capsq", 0 },
	{ "{capsr", 0 },
	{ "{capss", 0 },
	{ "{capst", 0 },
	{ "{capsu", 0 },
	{ "{capsv", 0 },
	{ "{capsw", 0 },
	{ "{capsx", 0 },
	{ "{capsy", 0 },
	{ "{capsz", 0 },
	{ "{crack1", 0 },
	{ "{crack2", 0 },
	{ "{crack3", 0 },
	{ "{crack4", 0 },
	{ "{crouch", 0 },
	{ "{dent1", 0 },
	{ "{dent2", 0 },
	{ "{dent3", 0 },
	{ "{dent4", 0 },
	{ "{dent5", 0 },
	{ "{dent6", 0 },
	{ "{ding1", 0 },
	{ "{ding10", 0 },
	{ "{ding11", 0 },
	{ "{ding2", 0 },
	{ "{ding3", 0 },
	{ "{ding4", 0 },
	{ "{ding5", 0 },
	{ "{ding6", 0 },
	{ "{ding7", 0 },
	{ "{ding8", 0 },
	{ "{ding9", 0 },
	{ "{drip2", 0 },
	{ "{drip3", 0 },
	{ "{drip4", 0 },
	{ "{drips1", 0 },
	{ "{explos", 0 },
	{ "{fault01", 0 },
	{ "{fault02", 0 },
	{ "{fault03", 0 },
	{ "{fault04", 0 },
	{ "{fault05", 0 },
	{ "{fault06", 0 },
	{ "{foot_l", 0 },
	{ "{foot_r", 0 },
	{ "{gaussshot1", 0 },
	{ "{graf001", 0 },
	{ "{graf002", 0 },
	{ "{graf003", 0 },
	{ "{graf004", 0 },
	{ "{graf005", 0 },
	{ "{hand1", 0 },
	{ "{large#s-", 0 },
	{ "{large#s0", 0 },
	{ "{large#s1", 0 },
	{ "{large#s2", 0 },
	{ "{large#s3", 0 },
	{ "{large#s4", 0 },
	{ "{large#s5", 0 },
	{ "{large#s6", 0 },
	{ "{large#s7", 0 },
	{ "{large#s8", 0 },
	{ "{large#s9", 0 },
	{ "{lime001", 0 },
	{ "{lime002", 0 },
	{ "{lime003", 0 },
	{ "{lime004", 0 },
	{ "{littleman", 0 },
	{ "{marker", 0 },
	{ "{med#s0", 0 },
	{ "{med#s1", 0 },
	{ "{med#s2", 0 },
	{ "{med#s3", 0 },
	{ "{med#s4", 0 },
	{ "{med#s5", 0 },
	{ "{med#s6", 0 },
	{ "{med#s7", 0 },
	{ "{med#s8", 0 },
	{ "{med#s9", 0 },
	{ "{moss001", 0 },
	{ "{moss002", 0 },
	{ "{moss003", 0 },
	{ "{moss004", 0 },
	{ "{moustache", 0 },
	{ "{oil1", 0 },
	{ "{oil2", 0 },
	{ "{pstripe1", 0 },
	{ "{pstripe2", 0 },
	{ "{pstripe3", 0 },
	{ "{pstripe4", 0 },
	{ "{rotatescrape", 0 },
	{ "{rust001", 0 },
	{ "{rust002", 0 },
	{ "{rust003", 0 },
	{ "{rust004", 0 },
	{ "{scorch3", 0 },
	{ "{small#s0", 0 },
	{ "{small#s1", 0 },
	{ "{small#s2", 0 },
	{ "{small#s3", 0 },
	{ "{small#s4", 0 },
	{ "{small#s5", 0 },
	{ "{small#s6", 0 },
	{ "{small#s7", 0 },
	{ "{small#s8", 0 },
	{ "{small#s9", 0 },
	{ "{stripeh", 0 },
	{ "{stripev", 0 },
	{ "{target", 0 },
	{ "{target2", 0 },
	{ "{tire1", 0 },
	{ "{tire2", 0 },
	{ "{turn1a", 0 },
	{ "{turn1b", 0 },
	{ "{turn1c", 0 },
	{ "{turn1d", 0 },
	{ "{turn2a", 0 },
	{ "{turn2b", 0 },
	{ "{turn2c", 0 },
	{ "{turn2d", 0 },
	{ "{water1", 0 },
	{ "{water2", 0 },
	{ "{water3", 0 },
};

void init_decals() {
	for (int i = 0; i < (int)ARRAYSIZE(gDecals); i++)
		gDecals[i].index = DECAL_INDEX(gDecals[i].name);
}

const char* get_decal_name(int idx) {
	for (int i = 0; i < (int)ARRAYSIZE(gDecals); i++) {
		if (gDecals[i].index == idx) {
			return gDecals[i].name;
		}
	}

	return NULL;
}

#define SF_DECAL_NOTINDEATHMATCH		2048

class CDecal : public CBaseEntity
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_LOW; }
	void	Spawn(void);
	void	KeyValue(KeyValueData* pkvd);
	void	EXPORT StaticDecal(void);
	void	EXPORT TriggerDecal(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
};

LINK_ENTITY_TO_CLASS(infodecal, CDecal)

// UNDONE:  These won't get sent to joining players in multi-player
void CDecal::Spawn(void)
{
	if (pev->skin < 0 || (gpGlobals->deathmatch && FBitSet(pev->spawnflags, SF_DECAL_NOTINDEATHMATCH)))
	{
		UTIL_Remove(this);
		return;
	}

	if (FStringNull(pev->targetname))
	{
		SetThink(&CDecal::StaticDecal);
		// if there's no targetname, the decal will spray itself on as soon as the world is done spawning.
		pev->nextthink = gpGlobals->time;
	}
	else
	{
		// if there IS a targetname, the decal sprays itself on when it is triggered.
		SetThink(&CDecal::SUB_DoNothing);
		SetUse(&CDecal::TriggerDecal);
	}
}

void CDecal::TriggerDecal(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// this is set up as a USE function for infodecals that have targetnames, so that the
	// decal doesn't get applied until it is fired. (usually by a scripted sequence)
	TraceResult trace;

	UTIL_TraceLine(pev->origin - Vector(5, 5, 5), pev->origin + Vector(5, 5, 5), ignore_monsters, ENT(pev), &trace);

	UTIL_BSPDecal(ENTINDEX(trace.pHit), pev->origin, pev->skin);

	SetThink(&CDecal::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.1;
}


void CDecal::StaticDecal(void)
{
	TraceResult trace;
	int			entityIndex, modelIndex;

	UTIL_TraceLine(pev->origin - Vector(5, 5, 5), pev->origin + Vector(5, 5, 5), ignore_monsters, ENT(pev), &trace);

	entityIndex = (short)ENTINDEX(trace.pHit);
	if (entityIndex)
		modelIndex = (int)VARS(trace.pHit)->modelindex;
	else
		modelIndex = 0;

	// TODO: selectively send decal messages when clients join instead
	// of calling the engine function which does that automatically
	if (entityIndex < MAX_LEGACY_CLIENT_ENTS) {
		if (pev->skin >= 222) {
			ALERT(at_error, "Attempted to draw invalid decal index %d\n", pev->skin);
			pev->skin = 0;
		}
		g_engfuncs.pfnStaticDecal(pev->origin, (int)pev->skin, entityIndex, modelIndex);
	}
	else {
		ALERT(at_error, "Failed to apply static decal. Entity index too high.");
	}

	SUB_Remove();
}


void CDecal::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "texture"))
	{
		pev->skin = DECAL_INDEX(pkvd->szValue);

		// Found
		if (pev->skin >= 0)
			return;
		ALERT(at_console, "Can't find decal %s\n", pkvd->szValue);
	}
	else
		CBaseEntity::KeyValue(pkvd);
}
