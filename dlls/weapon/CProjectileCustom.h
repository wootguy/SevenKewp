#pragma once
#include "custom_weapon.h"

#define FL_WC_PROJ_NO_BUBBLES		1
#define FL_WC_PROJ_NO_ORIENT		2 // don't orient projectile to movement direction
#define FL_WC_PROJ_IS_HOOK			4 // projectile is a hook attached to the player

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

	void Spawn();

	void MoveThink();

	void Remove();

	void DamageTarget(CBaseEntity* ent);

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