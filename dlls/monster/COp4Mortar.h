#pragma once
#include "extdll.h"
#include "util.h"
#include "CGrenade.h"

class EXPORT CMortarShell : public CGrenade
{
public:
	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void Precache() override;
	void Spawn() override;

	void BurnThink();

	void FlyThink();

	void MortarExplodeTouch(CBaseEntity* pOther);

	static CMortarShell* CreateMortarShell(Vector vecOrigin, Vector vecAngles, CBaseEntity* pOwner, int velocity);

	int m_iTrail;
	float m_flIgniteTime;
	int m_velocity;
	bool m_iSoundedOff;
};


class EXPORT COp4Mortar : public CBaseMonster
{
public:
	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void KeyValue(KeyValueData* pkvd) override;

	void Precache() override;

	void Spawn() override;

	int ObjectCaps() override { return 0; }

	int	Classify(void);

	void MortarThink();
	void DropInit();

	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

	void PlaySound();

	void Off();

	void AIUpdatePosition();

	CBaseEntity* FindTarget();

	void UpdatePosition(int direction, int controller);

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	int d_x;
	int d_y;
	float m_lastupdate;
	bool m_playsound;
	int m_updated;
	int m_direction;
	Vector m_start;
	Vector m_end;
	int m_velocity;
	int m_hmin;
	int m_hmax;
	float m_fireLast;
	float m_maxRange;
	float m_minRange;
	int m_iEnemyType;
	float m_fireDelay;
	float m_trackDelay;
	bool m_tracking;
	float m_zeroYaw;
	Vector m_vGunAngle;
	Vector m_vIdealGunVector;
	Vector m_vIdealGunAngle;
};


class EXPORT COp4MortarController : public CBaseToggle
{
public:
	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	int ObjectCaps() override { return FCAP_CONTINUOUS_USE; }

	void KeyValue(KeyValueData* pkvd) override;

	void Spawn() override;

	void Reverse();

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	int m_direction;
	int m_controller;
};
