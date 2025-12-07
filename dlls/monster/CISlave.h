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

#define		ISLAVE_MAX_BEAMS	8

class EXPORT CISlave : public CTalkSquadMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int	 ISoundMask( void );
	int  Classify ( void );
	const char* DisplayName();
	int  IRelationship( CBaseEntity *pTarget );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	BOOL CheckRangeAttack2 ( float flDot, float flDist );
	void CallForHelp( const char *szClassname, float flDist, EHANDLE hEnemy, Vector &vecLocation );
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);

	void DeathSound( void );
	void PainSound( void );
	void AlertSound( void );
	void IdleSound( void );
	void StartFollowingSound();
	void StopFollowingSound();
	void CantFollowSound();

	void Killed( entvars_t *pevAttacker, int iGib );

    void StartTask ( Task_t *pTask );
	void RunTask(Task_t* pTask);
	Schedule_t *GetSchedule( void );
	Schedule_t *GetScheduleOfType ( int Type );
	CUSTOM_SCHEDULES;

	int	Save( CSave &save ); 
	int Restore( CRestore &restore );
	static TYPEDESCRIPTION m_SaveData[];

	void ClearBeams( );
	void ArmBeam( int side );
	void WackBeam( int side, CBaseEntity *pEntity );
	void ZapBeam( int side );
	void BeamGlow( void );

	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	int m_iBravery;

	EHANDLE m_hBeam[ISLAVE_MAX_BEAMS];

	int m_iBeams;
	float m_flNextAttack;

	int	m_voicePitch;

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
	int	Classify(void) {
		return	CBaseMonster::Classify(CLASS_ALIEN_PASSIVE);
	}
	void KeyValue(KeyValueData* pkvd) override;

	int m_iPose; // which sequence to display	-- temporary, don't need to save
	static const char* m_szPoses[1];
};
