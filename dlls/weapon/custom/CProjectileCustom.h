#pragma once
#include "wc_params.h"
#include "../dlls/CBaseAnimating.h"

class EXPORT CProjectileCustom : public CBaseAnimating
{
public:
	float thinkDelay;
	EHANDLE spriteAttachment; // we'll need to kill this before we die (lol murder)
	bool attached;
	EHANDLE attachTarget; // entity attached to
	Vector attachStartOri; // Our initial position when attaching to the entity
	Vector targetStartOri; // initial position of the entity we attached to
	Vector attachStartDir; // our initial direction when attaching to the entity
	int attachTime;

	int flags; // FL_WC_PROJ_*
	WeaponCustomProjectileAction world_event;
	WeaponCustomProjectileAction monster_event;
	float expire_time;
	float air_friction;
	float water_friction;
	string_t move_snd;
	float damage;
	int damageType;	

	bool move_snd_playing;
	float nextBubbleTime;
	float bubbleDelay;
	int m_maxFrame;

	void Spawn();

	void Precache();

	void MoveThink();

	void Remove();

	virtual void DamageTarget(CBaseEntity* ent);

	void Touch(CBaseEntity* pOther);
	
	void PlayMoveSound();
	void StopMoveSound();

	CProjectileCustom* MyProjectileCustomPtr(void) override { return this; }

	WeaponCustomProjectileAction getImpactAction(CBaseEntity* pOther);

	// override in a child class to run custom logic
	virtual void CustomMove();
	virtual void CustomTouch(CBaseEntity* pOther);
	virtual void Impact(CBaseEntity* pOther);
	virtual void Bounce(CBaseEntity* pOther);
	virtual void Attach(CBaseEntity* pOther);
	virtual void Configure(CBasePlayer* attackr, CWeaponCustom* weapon, WepEvt& evt) {} // called before spawning
	virtual void Die() {} // called when life time expires
};