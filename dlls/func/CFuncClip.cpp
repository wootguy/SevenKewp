#include "extdll.h"
#include "util.h"
#include "cbase.h"

#define	SF_FCLIP_START_OFF 1
#define	SF_FCLIP_DIRECTIONAL 2
#define	SF_FCLIP_NO_CLIENTS 4
#define	SF_FCLIP_MONSTERS 8
#define	SF_FCLIP_PUSHABLES 16
#define	SF_FCLIP_EVERYTHING_ELSE 32

class CFuncClip : public CBaseEntity
{
public:
	int	GetEntindexPriority() { return ENTIDX_PRIORITY_LOW; }
	void Spawn(void);
	void Solidify(bool solid);
	void EXPORT DirectionalTouch(CBaseEntity* pOther);
	void EXPORT ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	Vector m_clipDirection;
};

LINK_ENTITY_TO_CLASS(func_clip, CFuncClip)

void CFuncClip::Spawn()
{
	Solidify(!(pev->spawnflags & SF_FCLIP_START_OFF));

	SET_MODEL(ENT(pev), STRING(pev->model));
	pev->effects |= EF_NODRAW;
	
	// use this instead of EF_NODRAW if you want client prediction for movement to work
	// but don't do it yet because it breaks weapon prediction
	//pev->rendermode = kRenderTransTexture;
	//pev->renderamt = 0;

	if (pev->spawnflags & SF_FCLIP_NO_CLIENTS) {
		pev->flags |= FL_NOCLIP_PLAYERS;
	}
	if (!(pev->spawnflags & SF_FCLIP_MONSTERS)) {
		pev->flags |= FL_NOCLIP_MONSTERS;
	}
	if (!(pev->spawnflags & SF_FCLIP_PUSHABLES)) {
		pev->flags |= FL_NOCLIP_PUSHABLES;
	}
	if (!(pev->spawnflags & SF_FCLIP_EVERYTHING_ELSE)) {
		pev->flags |= FL_NOCLIP_EVERYTHING_ELSE;
	}
	pev->flags |= FL_NOCLIP_TRACES;

	SetUse(&CFuncClip::ToggleUse);

	UTIL_SetOrigin(pev, pev->origin);		// Link into the list

	MAKE_VECTORS(pev->angles);
	m_clipDirection = gpGlobals->v_forward;
}

void CFuncClip::ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	Solidify(pev->solid == SOLID_NOT);
	UTIL_SetOrigin(pev, pev->origin);
}

void CFuncClip::Solidify(bool solid) {
	if (solid) {
		if (pev->spawnflags & SF_FCLIP_DIRECTIONAL) {
			// can't use normal engine physics for this
			pev->solid = SOLID_TRIGGER;
			pev->movetype = MOVETYPE_NONE;
			SetTouch(&CFuncClip::DirectionalTouch);
		}
		else {
			pev->solid = SOLID_BSP;
			pev->movetype = MOVETYPE_PUSH;
			SetTouch(NULL);
		}
	}
	else {
		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NONE;
		SetTouch(NULL);
	}
}

void CFuncClip::DirectionalTouch(CBaseEntity* pOther)
{
	if (pOther->IsPlayer()) {
		if (pev->spawnflags & SF_FCLIP_NO_CLIENTS)
			return;
	}
	else if (pOther->IsMonster()) {
		if (!(pev->spawnflags & SF_FCLIP_MONSTERS))
			return;
	}
	else if (pev->movetype == MOVETYPE_PUSHSTEP) {
		if (!(pev->spawnflags & SF_FCLIP_PUSHABLES))
			return;
	}
	else if (!(pev->spawnflags & SF_FCLIP_EVERYTHING_ELSE)) {
		return;
	}

	Vector movement = (pOther->pev->origin - pOther->pev->oldorigin);
	Vector velocity = pOther->pev->velocity;

	float backoffMove = DotProduct(movement, m_clipDirection);
	float backoffVel = DotProduct(pOther->pev->velocity, m_clipDirection);
	
	if (backoffMove < 0.0f) {
		pOther->pev->origin = pOther->pev->oldorigin + (movement - (m_clipDirection * backoffMove));
	}

	if (backoffVel < 0.0f) {
		pOther->pev->velocity = velocity - (m_clipDirection * backoffVel);
	}
}

