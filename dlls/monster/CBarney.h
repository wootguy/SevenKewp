#pragma once
#include	"extdll.h"
#include	"util.h"
#include	"monsters.h"
#include	"CTalkSquadMonster.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
// first flag is barney dying for scripted sequences?
#define		BARNEY_AE_DRAW		( 2 )
#define		BARNEY_AE_SHOOT		( 3 )
#define		BARNEY_AE_HOLSTER	( 4 )

#define	BARNEY_BODY_GUNHOLSTERED	0
#define	BARNEY_BODY_GUNDRAWN		1
#define BARNEY_BODY_GUNGONE			2

class EXPORT CBarney : public CTalkSquadMonster
{
public:
	void Spawn( void ) override;
	void Precache( void ) override;
	void SetYawSpeed( void ) override;
	int  ISoundMask( void ) override;
	void BarneyFirePistol( void );
	void AlertSound( void ) override;
	int  Classify ( void ) override;
	const char* DisplayName() override;
	void HandleAnimEvent( MonsterEvent_t *pEvent ) override;
	
	void RunTask( Task_t *pTask ) override;
	void StartTask( Task_t *pTask ) override;
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	BOOL CheckRangeAttack1 ( float flDot, float flDist ) override;
	
	void DeclineFollowing( void ) override;

	// Override these to set behavior
	Schedule_t *GetScheduleOfType ( int Type ) override;
	Schedule_t *GetSchedule ( void ) override;
	MONSTERSTATE GetIdealState ( void ) override;

	void DeathSound( void ) override;
	void PainSound( void ) override;
	
	void TalkInit( void );

	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) override;
	void Killed( entvars_t *pevAttacker, int iGib ) override;
	const char* GetDeathNoticeWeapon() override { return "weapon_9mmhandgun"; }
	virtual BOOL IsBarnacleFood(void) override { return TRUE; }
	
	virtual int		Save( CSave &save ) override;
	virtual int		Restore( CRestore &restore ) override;
	static	TYPEDESCRIPTION m_SaveData[];

	BOOL	m_fGunDrawn;
	float	m_painTime;
	float	m_checkAttackTime;
	BOOL	m_lastAttackCheck;

	// UNDONE: What is this for?  It isn't used?
	float	m_flPlayerDamage;// how much pain has the player inflicted on me?

	CUSTOM_SCHEDULES;

private:
	static const char* pPainSounds[];
	static const char* pDieSounds[];
};


//=========================================================
// DEAD BARNEY PROP
//
// Designer selects a pose in worldcraft, 0 through num_poses-1
// this value is added to what is selected as the 'first dead pose'
// among the monster's normal animations. All dead poses must
// appear sequentially in the model file. Be sure and set
// the m_iFirstPose properly!
//
//=========================================================
class EXPORT CDeadBarney : public CBaseMonster
{
public:
	void Spawn( void ) override;
	int	Classify ( void ) override { return	CLASS_PLAYER_ALLY; }

	void KeyValue( KeyValueData *pkvd ) override;

	int	m_iPose;// which sequence to display	-- temporary, don't need to save
	static const char *m_szPoses[3];
};
