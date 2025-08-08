/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

/*
==========================
This file contains "stubs" of class member implementations so that we can predict certain
 weapons client side.  From time to time you might find that you need to implement part of the
 these functions.  If so, cut it from here, paste it in hl_weapons.cpp or somewhere else and
 add in the functionality you need.
==========================
*/
#include	"extdll.h"
#include	"util.h"
#include	"CWorld.h"
#include	"CBasePlayer.h"
#include	"CMultiSource.h"
#include	"weapon/weapons.h"
#include	"nodes.h"
#include	"env/CSoundEnt.h"
#include	"skill.h"
#include	"weapon/CBasePlayerAmmo.h"
#include	"weapon/CBasePlayerItem.h"
#include	"weapon/CBasePlayerWeapon.h"
#include	"weapon/CGrenade.h"
#include	"env/effects.h"

// Globals used by game logic
const Vector g_vecZero = Vector( 0, 0, 0 );
int gmsgWeapPickup = 0;
enginefuncs_t g_engfuncs;
globalvars_t  *gpGlobals;

ItemInfo CBasePlayerItem::ItemInfoArray[MAX_WEAPONS];

StringMap g_soundReplacements;

void EMIT_SOUND_DYN(edict_t *entity, int channel, const char *sample, float volume, float attenuation, int flags, int pitch) { }
char* UTIL_VarArgs(const char* format, ...) { return ""; }
void UTIL_ClientPrint(CBaseEntity* client, PRINT_TYPE print_type, const char* msg) {}
const char* UTIL_SevenKewpClientString(int version) { return ""; }

// CBaseEntity Stubs
int CBaseEntity :: TakeHealth( float flHealth, int bitsDamageType, float healthcap) { return 1; }
int CBaseEntity :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType ) { return 1; }
CBaseEntity *CBaseEntity::GetNextTarget( void ) { return NULL; }
int CBaseEntity::Save( CSave &save ) { return 1; }
int CBaseEntity::Restore( CRestore &restore ) { return 1; }
void CBaseEntity::SetObjectCollisionBox( void ) { }
int	CBaseEntity :: Intersects( CBaseEntity *pOther ) { return 0; }
void CBaseEntity :: MakeDormant( void ) { }
int CBaseEntity :: IsDormant( void ) { return 0; }
BOOL CBaseEntity :: IsInWorld( void ) { return TRUE; }
int CBaseEntity::ShouldToggle( USE_TYPE useType, BOOL currentState ) { return 0; }
int	CBaseEntity :: DamageDecal( int bitsDamageType ) { return -1; }
CBaseEntity * CBaseEntity::Create( const char *szName, const Vector &vecOrigin, const Vector &vecAngles, bool spawn, edict_t *pentOwner, const StringMap& keys) { return NULL; }
void CBaseEntity::SUB_Remove( void ) { }
void CBaseEntity::UpdateOnRemove( void ) { }
void CBaseMonster::UpdateOnRemove( void ) { }
void CBaseEntity::SetClassify( int ) { }
void CBaseEntity::Spawn() { }
void CBaseEntity::Precache() { }
const char* CBaseEntity::DisplayName() { return NULL; }
bool CBaseEntity::BreakableUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) { return false; }
void CBaseEntity::BreakableDie(CBaseEntity* pActivator) { }
void CBaseEntity::BreakableDamageSound() { }
bool CBaseEntity::IsDelaySpawned() { return 0; }
Vector CBaseEntity::GetLookDirection() { return g_vecZero; }
Vector CBasePlayer::GetLookDirection() { return g_vecZero; }

// CBaseDelay Stubs
void CBaseDelay :: KeyValue( struct KeyValueData_s * ) { }
int CBaseDelay::Restore( class CRestore & ) { return 1; }
int CBaseDelay::Save( class CSave & ) { return 1; }

// CBaseAnimating Stubs
int CBaseAnimating::Restore( class CRestore & ) { return 1; }
int CBaseAnimating::Save( class CSave & ) { return 1; }

// DEBUG Stubs
edict_t *DBG_EntOfVars( const entvars_t *pev ) { return NULL; }
void DBG_AssertFunction(BOOL fExpr,	const char*	szExpr,	const char*	szFile,	int szLine,	const char*	szMessage) { }

// UTIL_* Stubs
void UTIL_PrecacheOther( const char *szClassname ) { }
void UTIL_PrecacheOther( const char *szClassname, const StringMap& keys ) { }
void UTIL_BloodDrips( const Vector &origin, const Vector &direction, int color, int amount ) { }
edict_t* UTIL_DecalTrace(TraceResult* pTrace, int decalNumber) { return NULL; }
edict_t* UTIL_GunshotDecalTrace( TraceResult *pTrace, int decalNumber ) { return NULL; }
void UTIL_MakeVectors( const Vector &vecAngles ) { }
BOOL UTIL_IsValidEntity( edict_t *pent ) { return TRUE; }
void UTIL_SetOrigin( entvars_t *, const Vector &org ) { }
BOOL UTIL_GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon ) { return TRUE; }
void UTIL_LogPrintf(char *,...) { }
void UTIL_ClientPrintAll( int,char const *,char const *,char const *,char const *,char const *) { }
void ClientPrint( entvars_t *client, int msg_dest, const char *msg_name, const char *param1, const char *param2, const char *param3, const char *param4 ) { }

// CBaseToggle Stubs
int CBaseToggle::Restore( class CRestore & ) { return 1; }
int CBaseToggle::Save( class CSave & ) { return 1; }
void CBaseToggle :: KeyValue( struct KeyValueData_s * ) { }
void CBaseToggle :: InitStateTriggers() { }
void CBaseToggle :: FireStateTriggers() { }

// CGrenade Stubs
void CGrenade::BounceSound( void ) { }
void CGrenade::Explode( Vector, Vector ) { }
void CGrenade::Explode( TraceResult *, int ) { }
void CGrenade::Killed( entvars_t *, int ) { }
void CGrenade::Spawn( void ) { }
const char* CGrenade::GetModel(void) { return 0; }
CGrenade * CGrenade:: ShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time, const char* model ){ return 0; }
CGrenade *CGrenade::ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity ){ return 0; }
void CGrenade::DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ){ }

void UTIL_Remove( CBaseEntity *pEntity ){ }
struct skilldata_t  gSkillData;
void UTIL_SetSize( entvars_t *pev, const Vector &vecMin, const Vector &vecMax ){ }
CBaseEntity *UTIL_FindEntityInSphere( CBaseEntity *pStartEntity, const Vector &vecCenter, float flRadius ){ return 0;}
CBaseEntity *UTIL_FindEntityByClassname(CBaseEntity* pStartEntity, const char* szName){ return 0;}

Vector UTIL_VecToAngles( const Vector &vec ){ return 0; }
CSprite *CSprite::SpriteCreate( const char *pSpriteName, const Vector &origin, BOOL animate ) { return 0; }
void CBeam::PointEntInit( const Vector &start, int endIndex ) { }
void CBeam::Spawn() { }
void CBeam::Precache() { }
void CBeam::BeamDamage(TraceResult* ptr) {}
const Vector& CBeam::GetStartPos(void) { return g_vecZero; }
const Vector& CBeam::GetEndPos(void) { return g_vecZero; }
CBeam *CBeam::BeamCreate( const char *pSpriteName, int width ) { return NULL; }
void CSprite::Expand( float scaleSpeed, float fadeSpeed ) { }


CBaseEntity* CBaseMonster :: CheckTraceHullAttack( float flDist, int iDamage, int iDmgType ) { return NULL; }
void CBaseMonster :: Eat ( float flFullDuration ) { }
BOOL CBaseMonster :: FShouldEat ( void ) { return TRUE; }
void CBaseMonster :: BarnacleVictimBitten ( entvars_t *pevBarnacle ) { }
void CBaseMonster :: BarnacleVictimReleased ( void ) { }
void CBaseMonster :: Listen ( void ) { }
float CBaseMonster :: FLSoundVolume ( CSound *pSound ) { return 0.0; }
BOOL CBaseMonster :: FValidateHintType ( short sHint ) { return FALSE; }
void CBaseMonster :: Look ( int iDistance ) { }
int CBaseMonster :: ISoundMask ( void ) { return 0; }
int CBaseMonster :: Classify ( void ) { return 0; }
CSound* CBaseMonster :: PBestSound ( void ) { return NULL; }
CSound* CBaseMonster :: PBestScent ( void ) { return NULL; } 
float CBaseAnimating :: StudioFrameAdvance ( float flInterval ) { return 0.0; }
void CBaseMonster :: MonsterThink ( void ) { }
void CBaseMonster :: MonsterUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value ) { }
int CBaseMonster :: IgnoreConditions ( void ) { return 0; }
void CBaseMonster :: RouteClear ( void ) { }
void CBaseMonster :: RouteNew ( void ) { }
BOOL CBaseMonster :: FRouteClear ( void ) { return FALSE; }
BOOL CBaseMonster :: FRefreshRoute ( void ) { return 0; }
BOOL CBaseMonster::MoveToEnemy( Activity movementAct, float waitTime ) { return FALSE; }
BOOL CBaseMonster::MoveToLocation( Activity movementAct, float waitTime, const Vector &goal ) { return FALSE; }
BOOL CBaseMonster::MoveToTarget( Activity movementAct, float waitTime ) { return FALSE; }
BOOL CBaseMonster::MoveToNode( Activity movementAct, float waitTime, const Vector &goal ) { return FALSE; }
int ShouldSimplify( int routeType ) { return TRUE; }
void CBaseMonster :: RouteSimplify( CBaseEntity *pTargetEnt ) { }
BOOL CBaseMonster :: BarnacleVictimCaught ( void ) { return TRUE; }
BOOL CBaseMonster :: CheckRangeAttack1 ( float flDot, float flDist ) { return FALSE; }
BOOL CBaseMonster :: CheckRangeAttack2 ( float flDot, float flDist ) { return FALSE; }
BOOL CBaseMonster :: CheckMeleeAttack1 ( float flDot, float flDist ) { return FALSE; }
BOOL CBaseMonster :: CheckMeleeAttack2 ( float flDot, float flDist ) { return FALSE; }
void CBaseMonster :: CheckAttacks ( CBaseEntity *pTarget, float flDist ) { }
BOOL CBaseMonster :: FCanCheckAttacks ( void ) { return FALSE; }
int CBaseMonster :: CheckEnemy ( CBaseEntity *pEnemy ) { return 0; }
void CBaseMonster :: PushEnemy( CBaseEntity *pEnemy, Vector &vecLastKnownPos ) { }
BOOL CBaseMonster :: PopEnemy( ) { return FALSE; }
void CBaseMonster :: SetActivity ( Activity NewActivity ) { }
void CBaseMonster :: SetSequenceByName ( const char *szSequence ) { }
int CBaseMonster :: CheckLocalMove ( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, float *pflDist ) { return 0; }
float CBaseMonster :: OpenDoorAndWait( entvars_t *pevDoor ) { return 0.0; }
void CBaseMonster :: AdvanceRoute ( float distance ) { }
int CBaseMonster :: RouteClassify( int iMoveFlag ) { return 0; }
BOOL CBaseMonster :: BuildRoute ( const Vector &vecGoal, int iMoveFlag, CBaseEntity *pTarget, bool useNodes ) { return FALSE; }
void CBaseMonster :: InsertWaypoint ( Vector vecLocation, int afMoveFlags ) { }
BOOL CBaseMonster :: FTriangulate ( const Vector &vecStart , const Vector &vecEnd, float flDist, CBaseEntity *pTargetEnt, Vector *pApex ) { return FALSE; }
void CBaseMonster :: Move ( float flInterval ) { }
BOOL CBaseMonster:: ShouldAdvanceRoute( float flWaypointDist ) { return FALSE; }
void CBaseMonster::MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval ) { }
void CBaseMonster :: MonsterInit ( void ) { }
void CBaseMonster :: UnstuckSpawnPosition ( void ) { }
void CBaseMonster :: MonsterInitThink ( void ) { }
void CBaseMonster :: StartMonster ( void ) { }
void CBaseMonster :: MovementComplete( void ) { }
int CBaseMonster::TaskIsRunning( void ) { return 0; }
void CBaseMonster::ScheduleChange( void ) { }
void CBaseMonster::SetClassify( int ) { }
void CBaseMonster::Revive( void ) { }
void CBaseMonster::Provoke(CBaseEntity* attacker) {}
void CBaseMonster::OnKillProvoker(CBaseEntity* provoker) {}
void CBaseMonster::Unprovoke(bool friendsToo) {}
int CBaseMonster::IRelationship(CBaseEntity* pTarget) { return 0; }
float CBaseMonster::GetDamage(float defaultDamage) { return defaultDamage; }
int CBaseEntity::IRelationship ( CBaseEntity *pTarget ) { return 0; }
BOOL CBaseMonster :: FindCover ( Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist ) { return FALSE; }
BOOL CBaseMonster :: BuildNearestRoute ( Vector vecThreat, Vector vecViewOffset, float flMinDist, float flMaxDist ) { return FALSE; }
CBaseEntity *CBaseMonster :: BestVisibleEnemy ( void ) { return NULL; }
BOOL CBaseMonster :: FInViewCone ( CBaseEntity *pEntity ) { return FALSE; }
BOOL CBaseMonster :: FInViewCone ( Vector *pOrigin ) { return FALSE; }
BOOL CBaseEntity :: FVisible ( CBaseEntity *pEntity, bool fIgnoreGlass ) { return FALSE; }
BOOL CBaseEntity :: FVisible ( const Vector &vecOrigin, bool fIgnoreGlass) { return FALSE; }
void CBaseMonster :: MakeIdealYaw( Vector vecTarget ) { }
float	CBaseMonster::FlYawDiff ( void ) { return 0.0; }
float CBaseMonster::ChangeYaw ( int yawSpeed ) { return 0; }
float	CBaseMonster::VecToYaw ( Vector vecDir ) { return 0.0; }
int CBaseAnimating :: LookupActivity ( int activity ) { return 0; }
int CBaseAnimating :: LookupActivityHeaviest ( int activity ) { return 0; }
void CBaseMonster :: SetEyePosition ( void ) { }
int CBaseAnimating :: LookupSequence ( const char *label ) { return 0; }
void CBaseAnimating :: ResetSequenceInfo ( ) { }
BOOL CBaseAnimating :: GetSequenceFlags( ) { return FALSE; }
void CBaseAnimating :: DispatchAnimEvents ( float flInterval ) { }
void CBaseMonster :: HandleAnimEvent( MonsterEvent_t *pEvent ) { }
float CBaseAnimating :: SetBoneController ( int iController, float flValue ) { return 0.0; }
void CBaseAnimating :: InitBoneControllers ( void ) { }
float CBaseAnimating :: SetBlending ( int iBlender, float flValue ) { return 0; }
void CBaseAnimating :: GetBonePosition ( int iBone, Vector &origin, Vector &angles ) { }
void CBaseAnimating :: GetAttachment ( int iAttachment, Vector &origin, Vector &angles ) { }
int CBaseAnimating :: FindTransition( int iEndingSequence, int iGoalSequence, int *piDir ) { return -1; }
void CBaseAnimating :: GetAutomovement( Vector &origin, Vector &angles, float flInterval ) { }
void CBaseAnimating :: SetBodygroup( int iGroup, int iValue ) { }
int CBaseAnimating :: GetBodygroup( int iGroup ) { return 0; }
int CBaseAnimating :: GetBodygroups( ) { return 0; }
Vector CBaseMonster :: GetGunPosition( void ) { return g_vecZero; }
void CBaseEntity::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) { }
void CBaseEntity::FireBullets(ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFreq, int iDamage, entvars_t *pevAttacker ) { }
void CBaseEntity :: TraceBleed( float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType ) { }
void CBaseMonster :: MakeDamageBloodDecal ( int cCount, float flNoise, TraceResult *ptr, const Vector &vecDir ) { }
BOOL CBaseMonster :: FGetNodeRoute ( Vector vecDest ) { return TRUE; }
int CBaseMonster :: FindHintNode ( void ) { return NO_NODE; }
void CBaseMonster::ReportAIState( void ) { }
void CBaseMonster :: KeyValue( KeyValueData *pkvd ) { }
BOOL CBaseMonster :: FCheckAITrigger ( void ) { return FALSE; }
int CBaseMonster :: CanPlaySequence( BOOL fDisregardMonsterState, int interruptLevel ) { return FALSE; }
BOOL CBaseMonster :: FindLateralCover ( const Vector &vecThreat, const Vector &vecViewOffset ) { return FALSE; }
Vector CBaseMonster :: ShootAtEnemy( const Vector &shootOrigin ) { return g_vecZero; }
BOOL CBaseMonster :: FacingIdeal( void ) { return FALSE; }
BOOL CBaseMonster :: FCanActiveIdle ( void ) { return FALSE; }
void CBaseMonster::CorpseFallThink( void ) { }
void CBaseMonster :: MonsterInitDead( void ) { }
BOOL CBaseMonster :: BBoxFlat ( void ) { return TRUE; }
BOOL CBaseMonster :: GetEnemy ( void ) { return FALSE; }
void CBaseMonster :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) { }
CBaseEntity* CBaseMonster :: DropItem ( const char *pszItemName, const Vector &vecPos, const Vector &vecAng ) { return NULL; }
BOOL CBaseMonster :: ShouldFadeOnDeath( void ) { return FALSE; }
void CBaseMonster :: RadiusDamage(entvars_t* pevInflictor, entvars_t*	pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType ) { }
void CBaseMonster :: RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType ) { }
void CBaseMonster::FadeMonster( void ) { }
void CBaseMonster :: GibMonster( void ) { }
BOOL CBaseMonster :: HasHumanGibs( void ) { return FALSE; }
BOOL CBaseMonster :: HasAlienGibs( void ) { return FALSE; }
Activity CBaseMonster :: GetDeathActivity ( void ) { return ACT_DIE_HEADSHOT; }
MONSTERSTATE CBaseMonster :: GetIdealState ( void ) { return MONSTERSTATE_ALERT; }
Schedule_t* CBaseMonster :: GetScheduleOfType ( int Type ) { return NULL; }
Schedule_t *CBaseMonster :: GetSchedule ( void ) { return NULL; }
void CBaseMonster :: RunTask ( Task_t *pTask ) { }
void CBaseMonster :: StartTask ( Task_t *pTask ) { }
Schedule_t *CBaseMonster::ScheduleFromName( const char *pName ) { return NULL;}
void CBaseMonster::BecomeDead( void ) {}
void CBaseMonster :: RunAI ( void ) {}
void CBaseMonster :: Killed( entvars_t *pevAttacker, int iGib ) {}
int CBaseMonster :: TakeHealth (float flHealth, int bitsDamageType, float healthcap) { return 0; }
int CBaseMonster :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType ) { return 0; }
int CBaseMonster::Restore( class CRestore & ) { return 1; }
int CBaseMonster::Save( class CSave & ) { return 1; }
void CBaseMonster::StopFollowing(BOOL clearSchedule) {}
void CBaseMonster::StartFollowing(CBaseEntity* pLeader) {}
const char* CBaseMonster::GetTaskName(int taskIdx) {return "";}
const char* CBaseMonster::DisplayName() {return "";}
BOOL CBaseMonster::IsMachine() {return 0;}
void CBaseMonster::Precache() {}
void CBaseMonster::GetAllSchedules(std::unordered_set<Schedule_t*>& schedulesOut) {}
Schedule_t* CBaseMonster::ScheduleFromTableIdx(uint32_t idx) { return NULL; }
int CBaseMonster::GetScheduleTableSize() { return 0; }
int CBaseMonster::GetScheduleTableIdx() { return 0; }
float CBaseMonster::GetDamageModifier() { return 0; }
BOOL CBaseMonster::NoFriendlyFire() { return 0; }
void CBaseMonster::SetClassification(int newclass) { }

int TrainSpeed(int iSpeed, int iMax) { 	return 0; }
void CBasePlayer :: DeathSound( void ) { }
int CBasePlayer :: TakeHealth( float flHealth, int bitsDamageType, float healthcap) { return 0; }
void CBasePlayer :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) { }
int CBasePlayer :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType ) { return 0; }
void CBasePlayer::PackDeadPlayerItems( void ) { }
void CBasePlayer::RemoveAllItems( BOOL removeSuit, BOOL dontHideItems ) { }
void CBasePlayer::SetAnimation( PLAYER_ANIM playerAnim, float duration ) { }
void CBasePlayer::WaterMove() { }
BOOL CBasePlayer::IsOnLadder( void ) { return FALSE; }
void CBasePlayer::PlayerDeathThink(void) { }
void CBasePlayer::StartDeathCam( void ) { }
void CBasePlayer::StartObserver( Vector vecPosition, Vector vecViewAngle ) { }
void CBasePlayer::PlayerUse ( void ) { }
void CBasePlayer::Jump() { }
void CBasePlayer::Duck( ) { }
int  CBasePlayer::Classify ( void ) { return 0; }
void CBasePlayer::PreThink(void) { }
void CBasePlayer::CheckTimeBasedDamage()  { }
void CBasePlayer :: UpdateGeigerCounter( void ) { }
void CBasePlayer::CheckSuitUpdate() { }
void CBasePlayer::SetSuitUpdate(const char *name, int fgroup, int iNoRepeatTime) { }
void CBasePlayer :: UpdatePlayerSound ( void ) { }
void CBasePlayer::PostThink() { }
void CBasePlayer :: Precache( void ) { }
int CBasePlayer::Save( CSave &save ) { return 0; }
void CBasePlayer::RenewItems(void) { }
int CBasePlayer::Restore( CRestore &restore ) { return 0; }
void CBasePlayer::SelectNextItem( int iItem ) { }
BOOL CBasePlayer::HasWeapons( void ) { return FALSE; }
void CBasePlayer::SelectPrevItem( int iItem ) { }
CBaseEntity *FindEntityForward( CBaseEntity *pMe ) { return NULL; }
BOOL CBasePlayer :: FlashlightIsOn( void ) { return FALSE; }
void CBasePlayer :: FlashlightTurnOn( void ) { }
void CBasePlayer :: FlashlightTurnOff( void ) { }
void CBasePlayer :: ForceClientDllUpdate( void ) { }
void CBasePlayer::ImpulseCommands( ) { }
void CBasePlayer::CheatImpulseCommands( int iImpulse ) { }
int CBasePlayer::AddPlayerItem( CBasePlayerItem *pItem ) { return FALSE; }
int CBasePlayer::RemovePlayerItem( CBasePlayerItem *pItem ) { return FALSE; }
void CBasePlayer::ItemPreFrame() { }
void CBasePlayer::ItemPostFrame() { }
int CBasePlayer::AmmoInventory( int iAmmoIndex ) { return -1; }
int CBasePlayer::GetAmmoIndex(const char *psz) { return -1; }
void CBasePlayer::SendAmmoUpdate(void) { }
void CBasePlayer :: UpdateClientData( void ) { }
BOOL CBasePlayer ::BarnacleVictimCaught( void ) { return TRUE; }
void CBasePlayer :: BarnacleVictimBitten ( entvars_t *pevBarnacle ) { }
void CBasePlayer :: BarnacleVictimReleased ( void ) { }
int CBasePlayer :: Illumination( void ) { return 0; }
void CBasePlayer :: EnableControl(BOOL fControl) { }
Vector CBasePlayer :: GetAutoaimVector( float flDelta ) { return g_vecZero; }
Vector CBasePlayer :: AutoaimDeflection( Vector &vecSrc, float flDist, float flDelta  ) { return g_vecZero; }
void CBasePlayer :: ResetAutoaim( ) { }
void CBasePlayer :: ResetSequenceInfo( ) { }
void CBasePlayer :: SetCustomDecalFrames( int nFrames ) { }
int CBasePlayer :: GetCustomDecalFrames( void ) { return -1; }
void CBasePlayer::DropPlayerItem ( const char *pszItemName ) { }
BOOL CBasePlayer::HasPlayerItem( CBasePlayerItem *pCheckItem ) { return FALSE; }
BOOL CBasePlayer :: SwitchWeapon( CBasePlayerItem *pWeapon )  { return FALSE; }
Vector CBasePlayer :: GetGunPosition( void ) { return g_vecZero; }
const char *CBasePlayer::TeamID( void ) { return ""; }
int CBasePlayer :: GiveAmmo( int iCount, const char *szName, int iMax ) { return 0; }
void CBasePlayer::AddPoints( int score, BOOL bAllowNegativeScore ) { } 
void CBasePlayer::AddPointsToTeam( int score, BOOL bAllowNegativeScore ) { } 
Vector CBasePlayer::BodyTarget(const Vector& posSrc) { return Vector(); }
void CBasePlayer::Revive() { }
float CBasePlayer::GetDamage(float defaultDamage) { return defaultDamage; }
const char* CBasePlayer::GetDeathNoticeWeapon() { return 0; }
void CBasePlayer::TabulateAmmo() { }
CBasePlayerItem* CBasePlayer::GetNamedPlayerItem(const char* pszItemName) { return 0; }
BOOL CBasePlayer::HasNamedPlayerItem(const char* pszItemName) { return FALSE; }
void CBasePlayer::GiveNamedItem(const char* pszName) {}
void CBasePlayer::SendSevenKewpClientNotice() {}
bool CBasePlayer::IsSevenKewpClient() { return false; }

void ClearMultiDamage(void) { }
void ApplyMultiDamage(entvars_t *pevInflictor, entvars_t *pevAttacker ) { }
void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType) { }
void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage) { }
int DamageDecal( CBaseEntity *pEntity, int bitsDamageType ) { return 0; }
void DecalGunshot( TraceResult *pTrace, int iBulletType, bool playTextureSound, Vector vecSrc, Vector vecEnd, edict_t* emitter ) { }
void EjectBrass ( const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype ) { }
void AddAmmoNameToAmmoRegistry( const char *szAmmoname ) { }
int CBasePlayerItem::Restore( class CRestore & ) { return 1; }
int CBasePlayerItem::Save( class CSave & ) { return 1; }
int CBasePlayerWeapon::Restore( class CRestore & ) { return 1; }
int CBasePlayerWeapon::Save( class CSave & ) { return 1; }
float CBasePlayerWeapon::GetNextAttackDelay( float flTime ) { return flTime; }
void CBasePlayerItem :: SetObjectCollisionBox( void ) { }
void CBasePlayerItem :: FallInit( void ) { }
void CBasePlayerItem::FallThink ( void ) { }
void CBasePlayerItem::Materialize( void ) { }
void CBasePlayerItem::AttemptToMaterialize( void ) { }
void CBasePlayerItem :: CheckRespawn ( void ) { }
CBaseEntity* CBasePlayerItem::Respawn( void ) { return NULL; }
void CBasePlayerItem::DefaultTouch( CBaseEntity *pOther ) { }
void CBasePlayerItem::DestroyItem( void ) { }
int CBasePlayerItem::AddToPlayer( CBasePlayer *pPlayer ) { return TRUE; }
void CBasePlayerItem::Drop( void ) { }
void CBasePlayerItem::Kill( void ) { }
void CBasePlayerItem::Holster( int skiplocal ) { }
void CBasePlayerItem::AttachToPlayer ( CBasePlayer *pPlayer ) { }
void CBasePlayerItem::KeyValue(KeyValueData* pkvd) {}
int CBasePlayerItem::ObjectCaps(void) { return 0; }
int CBasePlayerAmmo::ObjectCaps(void) { return 0; }
int CBasePlayerWeapon::AddDuplicate( CBasePlayerItem *pOriginal ) { return 0; }
int CBasePlayerWeapon::AddToPlayer( CBasePlayer *pPlayer ) { return FALSE; }
int CBasePlayerWeapon::UpdateClientData( CBasePlayer *pPlayer ) { return 0; }
BOOL CBasePlayerWeapon :: AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry ) { return TRUE; }
BOOL CBasePlayerWeapon :: AddSecondaryAmmo( int iCount, char *szName, int iMax ) { return TRUE; }
BOOL CBasePlayerWeapon :: IsUseable( void ) { return TRUE; }
int CBasePlayerWeapon::PrimaryAmmoIndex( void ) { return -1; }
int CBasePlayerWeapon::SecondaryAmmoIndex( void ) {	return -1; }
void CBasePlayerAmmo::Spawn( void ) { }
CBaseEntity* CBasePlayerAmmo::Respawn( void ) { return this; }
void CBasePlayerAmmo::Materialize( void ) { }
void CBasePlayerAmmo :: DefaultTouch( CBaseEntity *pOther ) { }
void CBasePlayerAmmo::KeyValue(KeyValueData* pkvd) {}
int CBasePlayerWeapon::ExtractAmmo( CBasePlayerWeapon *pWeapon ) { return 0; }
int CBasePlayerWeapon::ExtractClipAmmo( CBasePlayerWeapon *pWeapon ) { return 0; }	
void CBasePlayerWeapon::RetireWeapon(void) { }
void CBasePlayerWeapon::KeyValue(KeyValueData* pkvd) {}
void CBasePlayerWeapon::Precache() {}
CBaseEntity* CBasePlayerWeapon::Respawn() { return NULL;  }
const char* CBasePlayerWeapon::GetModelV(const char*) { return 0; }
const char* CBasePlayerWeapon::GetModelP() { return 0; }
const char* CBasePlayerWeapon::GetModelW() { return 0; }
void CBasePlayerWeapon::SetWeaponModelW() { }
void CGrenade::SetGrenadeModel() { }
void CSoundEnt::InsertSound ( int iType, const Vector &vecOrigin, int iVolume, float flDuration ) {}
void CSoundEnt::Precache() {}
void CSoundEnt::Spawn() {}
void CSoundEnt::Think() {}
void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType ){}
void StartSound(edict_t* entity, int channel, const char* sample, float volume, float attenuation,
	int fFlags, int pitch, const float* origin, uint32_t messageTargets) {}
edict_t* EHANDLE::GetEdict(void) { return 0; }
CBaseEntity* EHANDLE::GetEntity(void) { return 0; }
void CBaseMonster::Nerf() {}
int CBaseMonster::HasTarget(string_t targetname) { return 0; }
CKeyValue CBaseEntity::GetKeyValue(const char* keyName) { return CKeyValue(); }
void CBaseEntity::KeyValue(KeyValueData* pkvd) {}
bool CBaseMonster::ShouldRoam() { return false; }
void CPointEntity::Spawn(void) {}
void CMultiSource::Spawn(void) {}
void CMultiSource::KeyValue(KeyValueData* pkvd) {}
void CMultiSource::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) {}
BOOL CMultiSource::IsTriggered(CBaseEntity* pActivator) { return 0; }
void CMultiSource::Register(void) {}
int	CMultiSource::Save(CSave& save) { return 0; }
int	CMultiSource::Restore(CRestore& restore) { return 0; }
void CWorld::Spawn(void) {}
void CWorld::Precache(void) {}
void CWorld::KeyValue(KeyValueData* pkvd) {}
void CWorld::loadReplacementFiles() {}
CBaseEntity* EHANDLE :: operator = (CBaseEntity* pEntity) { return 0; }
CBaseEntity* EHANDLE :: operator -> () { return 0; }
EHANDLE :: operator CBaseEntity* () { return 0; };

void CBaseToggle::PlaySentence(const char* pszSentence, float duration, float volume, float attenuation) {}
void CBaseToggle::PlayScriptedSentence(const char* pszSentence, float duration, float volume, float attenuation, BOOL bConcurrent, CBaseEntity* pListener) {}
void CBaseToggle::SentenceStop(void) {}
void CBasePlayerWeapon::GetAmmoDropInfo(bool isSecondary, const char*& ammoEntName, int& dropAmount) {}

void lagcomp_begin(CBasePlayer* plr) {}
void lagcomp_end() {}
void PLAY_DISTANT_SOUND(edict_t* emitter, int soundType) {}

CBaseEntity* RelocateEntIdx(CBaseEntity* pEntity) { return 0; }
void DEBUG_MSG(ALERT_TYPE target, const char* format, ...) {}