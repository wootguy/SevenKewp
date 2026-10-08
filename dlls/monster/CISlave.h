#pragma once
#include	"extdll.h"
#include	"util.h"
#include	"CTalkSquadMonster.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		ISLAVE_AE_CLAW		( 1 )
#define		ISLAVE_AE_CLAWRAKE	( 2 )
#define		ISLAVE_AE_ZAP_POWERUP	( 3 )
#define		ISLAVE_AE_ZAP_SHOOT		( 4 )
#define		ISLAVE_AE_ZAP_DONE		( 5 )

#define		ISLAVE_MAX_BEAMS	10

class EXPORT CISlave : public CTalkSquadMonster
{
public:
	void Spawn( void ) override;
	void Precache( void ) override;
	void SetYawSpeed( void ) override;
	int	 ISoundMask( void ) override;
	int  Classify ( void ) override;
	const char* DisplayName() override;
	int  IRelationship( CBaseEntity *pTarget ) override;
	void HandleAnimEvent( MonsterEvent_t *pEvent ) override;
	BOOL CheckRangeAttack1 ( float flDot, float flDist ) override;
	BOOL CheckRangeAttack2 ( float flDot, float flDist ) override;
	void CallForHelp( const char *szClassname, float flDist, EHANDLE hEnemy, Vector &vecLocation );
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) override;
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

	void DeathSound( void ) override;
	void PainSound( void ) override;
	void AlertSound( void ) override;
	void IdleSound( void ) override;
	void StartFollowingSound() override;
	void StopFollowingSound() override;
	void CantFollowSound() override;

	void Killed( entvars_t *pevAttacker, int iGib ) override;

    void StartTask ( Task_t *pTask ) override;
	void RunTask(Task_t* pTask) override;
	Schedule_t *GetSchedule( void ) override;
	Schedule_t *GetScheduleOfType ( int Type ) override;
	CUSTOM_SCHEDULES;

	int	Save( CSave &save ) override;
	int Restore( CRestore &restore ) override;
	static TYPEDESCRIPTION m_SaveData[];

	void ClearBeams( );
	void ArmBeam( int side );
	void WackBeam( int side, CBaseEntity *pEntity );
	void ZapBeam( int side, bool randomDir = false );
	void BeamGlow( void );

	const char* GetDeathNoticeWeapon() override { return "weapon_crowbar"; }
	virtual BOOL IsBarnacleFood(void) override { return TRUE; }

	int m_iBravery;

	EHANDLE m_hBeam[ISLAVE_MAX_BEAMS];

	int m_iBeams;
	float m_flNextAttack;

	int	m_voicePitch;
	int m_shockReactState;

	EHANDLE m_hDead;
	Vector m_beamColor;

	static const char *pAttackHitSounds[];
	static const char *pAttackMissSounds[];
	static const char *pPainSounds[];
	static const char *pDeathSounds[];
};

/**
 *	@brief DEAD ALIEN SLAVE PROP
 *	@details Designer selects a pose in worldcraft, 0 through num_poses-1
 *	this value is added to what is selected as the 'first dead pose' among the monster's normal animations.
 *	All dead poses must appear sequentially in the model file.
 *	Be sure and set the m_iFirstPose properly!
 */
class EXPORT CDeadISlave : public CBaseMonster
{
public:
	void Spawn() override;
	int	Classify(void) override {
		return	CBaseMonster::Classify(CLASS_ALIEN_PASSIVE);
	}
	void KeyValue(KeyValueData* pkvd) override;

	int m_iPose; // which sequence to display	-- temporary, don't need to save
	static const char* m_szPoses[1];
};
